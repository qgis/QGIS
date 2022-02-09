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
#include "qgsproject.h"

#include <cmath>
#include <QStandardItem>

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
  : QAbstractTableModel( parent )
{
}

void QgsGCPListModel::setGCPList( QgsGCPList *theGCPList )
{
  beginResetModel();
  mGCPList = theGCPList;
  endResetModel();
}

// ------------------------------- public ---------------------------------- //
void QgsGCPListModel::setGeorefTransform( QgsGeorefTransform *georefTransform )
{
  mGeorefTransform = georefTransform;
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, columnCount() - 1 ) );
}

int QgsGCPListModel::rowCount( const QModelIndex & ) const
{
  return mGCPList ? mGCPList->size() : 0;
}

int QgsGCPListModel::columnCount( const QModelIndex & ) const
{
  return static_cast< int >( Column::LastColumn ) + 1;
}

QVariant QgsGCPListModel::data( const QModelIndex &index, int role ) const
{
  if ( !mGCPList
       || index.row() < 0
       || index.row() >= mGCPList->size()
       || index.column() < 0
       || index.column() >= columnCount() )
    return QVariant();

  // TODO don't read this from settings!!
  const QgsCoordinateReferenceSystem targetCrs( QgsSettings().value( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ) ).toString() );

  const Column column = static_cast< Column >( index.column() );

  const QgsGeorefDataPoint *point = mGCPList->at( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
      switch ( column )
      {
        case QgsGCPListModel::Column::Enabled:
          break;
        case QgsGCPListModel::Column::ID:
          return index.row();
        case QgsGCPListModel::Column::SourceX:
          return point->sourcePoint().x(); // TODO formatting!!
        case QgsGCPListModel::Column::SourceY:
          return point->sourcePoint().y();
        case QgsGCPListModel::Column::DestinationX:
        {
          const QgsPointXY transformedDestinationPoint = point->transformedDestinationPoint( targetCrs, QgsProject::instance()->transformContext() );
          return transformedDestinationPoint.x();
        }
        case QgsGCPListModel::Column::DestinationY:
        {
          const QgsPointXY transformedDestinationPoint = point->transformedDestinationPoint( targetCrs, QgsProject::instance()->transformContext() );
          return transformedDestinationPoint.y();
        }
        case QgsGCPListModel::Column::ResidualDx:
          break;
        case QgsGCPListModel::Column::ResidualDy:
          break;
        case QgsGCPListModel::Column::TotalResidual:
          break;
        case QgsGCPListModel::Column::LastColumn:
          break;

      }
      break;

    case Qt::CheckStateRole:
      if ( column == Column::Enabled )
      {
        return point->isEnabled() ? Qt::Checked : Qt::Unchecked;
      }
      break;

    case static_cast< int >( Role::SourcePointRole ):
      return point->sourcePoint();

  }
  return QVariant();
}

Qt::ItemFlags QgsGCPListModel::flags( const QModelIndex &index ) const
{
  if ( !mGCPList
       || index.row() < 0
       || index.row() >= mGCPList->size()
       || index.column() < 0
       || index.column() >= columnCount() )
    return QAbstractTableModel::flags( index );

  const Column column = static_cast< Column >( index.column() );
  switch ( column )
  {
    case QgsGCPListModel::Column::Enabled:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsSelectable;

    case QgsGCPListModel::Column::ID:
    case QgsGCPListModel::Column::SourceX:
    case QgsGCPListModel::Column::SourceY:
    case QgsGCPListModel::Column::DestinationX:
    case QgsGCPListModel::Column::DestinationY:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;

    case QgsGCPListModel::Column::ResidualDx:
    case QgsGCPListModel::Column::ResidualDy:
    case QgsGCPListModel::Column::TotalResidual:
    case QgsGCPListModel::Column::LastColumn:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }
  return QAbstractTableModel::flags( index );
}

QVariant QgsGCPListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Horizontal:
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          QString residualUnitType;
          switch ( residualUnit() )
          {
            case QgsUnitTypes::RenderMapUnits:
              residualUnitType = tr( "map units" );
              break;
            case QgsUnitTypes::RenderPixels:
              residualUnitType = tr( "pixels" );
              break;

            case QgsUnitTypes::RenderMillimeters:
            case QgsUnitTypes::RenderPercentage:
            case QgsUnitTypes::RenderPoints:
            case QgsUnitTypes::RenderInches:
            case QgsUnitTypes::RenderUnknownUnit:
            case QgsUnitTypes::RenderMetersInMapUnits:
              break;
          }

          switch ( static_cast< Column >( section ) )
          {
            case QgsGCPListModel::Column::Enabled:
              return tr( "Enabled" );
            case QgsGCPListModel::Column::ID:
              return tr( "ID" );
            case QgsGCPListModel::Column::SourceX:
              return tr( "Source X" );
            case QgsGCPListModel::Column::SourceY:
              return tr( "Source Y" );
            case QgsGCPListModel::Column::DestinationX:
              return tr( "Dest. X" );
            case QgsGCPListModel::Column::DestinationY:
              return tr( "Dest. Y" );
            case QgsGCPListModel::Column::ResidualDx:
              return tr( "dX (%1)" ).arg( residualUnitType );
            case QgsGCPListModel::Column::ResidualDy:
              return tr( "dY (%1)" ).arg( residualUnitType );
            case QgsGCPListModel::Column::TotalResidual:
              return tr( "Residual (%1)" ).arg( residualUnitType );
            case QgsGCPListModel::Column::LastColumn:
              break;
          }
        }
        break;
        default:
          break;
      }

      break;
    case Qt::Vertical:
      break;
  }
  return QVariant();
}

QgsUnitTypes::RenderUnit QgsGCPListModel::residualUnit() const
{
  bool mapUnitsPossible = false;
  if ( mGeorefTransform )
  {
    mapUnitsPossible = mGeorefTransform->providesAccurateInverseTransformation();
  }

  if ( mapUnitsPossible && QgsSettings().value( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ) ) == "mapUnits" )
  {
    return QgsUnitTypes::RenderUnit::RenderMapUnits;
  }
  else
  {
    return QgsUnitTypes::RenderUnit::RenderPixels;
  }
}


void QgsGCPListModel::updateModel()
{
  if ( !mGCPList )
    return;

  bool bTransformUpdated = false;
  QVector<QgsPointXY> sourceCoordinates;
  QVector<QgsPointXY> destinationCoordinates;

  // TODO - don't read from settings
  const QgsCoordinateReferenceSystem targetCrs( QgsSettings().value( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ) ).toString() );
  mGCPList->createGCPVectors( sourceCoordinates, destinationCoordinates, targetCrs, QgsProject::instance()->transformContext() );

  if ( mGeorefTransform )
  {
    bTransformUpdated = mGeorefTransform->updateParametersFromGcps( sourceCoordinates, destinationCoordinates, true );
  }
  const QgsUnitTypes::RenderUnit unitType = residualUnit();

  // update residuals
  for ( int i = 0; i < mGCPList->size(); ++i )
  {
    QgsGeorefDataPoint *p = mGCPList->at( i );

    if ( !p )
      continue;

    p->setId( i );

    const QgsPointXY transformedDestinationPoint = p->transformedDestinationPoint( targetCrs, QgsProject::instance()->transformContext() );

    double residual = 0;
    double dX = 0;
    double dY = 0;
    // Calculate residual if transform is available and up-to-date
    if ( mGeorefTransform && bTransformUpdated && mGeorefTransform->parametersInitialized() )
    {
      QgsPointXY dst;
      const QgsPointXY pixel = mGeorefTransform->toSourcePixel( p->sourcePoint() );
      if ( unitType == QgsUnitTypes::RenderPixels )
      {
        // Transform from world to raster coordinate:
        // This is the transform direction used by the warp operation.
        // As transforms of order >=2 are not invertible, we are only
        // interested in the residual in this direction
        if ( mGeorefTransform->transformWorldToRaster( transformedDestinationPoint, dst ) )
        {
          dX = ( dst.x() - pixel.x() );
          dY = -( dst.y() - pixel.y() );
        }
      }
      else if ( unitType == QgsUnitTypes::RenderMapUnits )
      {
        if ( mGeorefTransform->transformRasterToWorld( pixel, dst ) )
        {
          dX = ( dst.x() - transformedDestinationPoint.x() );
          dY = ( dst.y() - transformedDestinationPoint.y() );
        }
      }
    }
    residual = std::sqrt( dX * dX + dY * dY );

    p->setResidual( QPointF( dX, dY ) );
#if 0
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
#endif
  }
}


