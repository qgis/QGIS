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

QgsGCPListModel::QgsGCPListModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

void QgsGCPListModel::setGCPList( QgsGCPList *theGCPList )
{
  beginResetModel();
  mGCPList = theGCPList;
  endResetModel();
  updateResiduals();
}

// ------------------------------- public ---------------------------------- //
void QgsGCPListModel::setGeorefTransform( QgsGeorefTransform *georefTransform )
{
  mGeorefTransform = georefTransform;
  updateResiduals();
}

void QgsGCPListModel::setTargetCrs( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context )
{
  mTargetCrs = targetCrs;
  mTransformContext = context;
  updateResiduals();
  emit dataChanged( index( 0, static_cast< int >( Column::DestinationX ) ),
                    index( rowCount() - 1, static_cast< int >( Column::DestinationY ) ) );
}

int QgsGCPListModel::rowCount( const QModelIndex & ) const
{
  return mGCPList ? mGCPList->size() : 0;
}

int QgsGCPListModel::columnCount( const QModelIndex & ) const
{
  return static_cast< int >( Column::LastColumn );
}

QVariant QgsGCPListModel::data( const QModelIndex &index, int role ) const
{
  if ( !mGCPList
       || index.row() < 0
       || index.row() >= mGCPList->size()
       || index.column() < 0
       || index.column() >= columnCount() )
    return QVariant();

  const Column column = static_cast< Column >( index.column() );

  const QgsGeorefDataPoint *point = mGCPList->at( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::UserRole:
    case Qt::ToolTipRole:
      switch ( column )
      {
        case QgsGCPListModel::Column::Enabled:
          break;
        case QgsGCPListModel::Column::ID:
          return index.row();

        case QgsGCPListModel::Column::SourceX:
        case QgsGCPListModel::Column::SourceY:
        {
          switch ( role )
          {
            case Qt::ToolTipRole:
            case Qt::EditRole:
            case Qt::UserRole:
              // use full precision
              return column == QgsGCPListModel::Column::SourceX ? point->sourcePoint().x() : point->sourcePoint().y();

            case Qt::DisplayRole:
              // truncate decimals for display
              return QString::number(
                       column == QgsGCPListModel::Column::SourceX ? point->sourcePoint().x() : point->sourcePoint().y(),
                       'f', 4 );
            default:
              break;
          }
          break;
        }

        case QgsGCPListModel::Column::DestinationX:
        case QgsGCPListModel::Column::DestinationY:
        {
          const QgsPointXY transformedDestinationPoint = point->transformedDestinationPoint( mTargetCrs, mTransformContext );

          switch ( role )
          {
            case Qt::ToolTipRole:
            case Qt::EditRole:
            case Qt::UserRole:
              // use full precision
              return column == QgsGCPListModel::Column::DestinationX ? transformedDestinationPoint.x() : transformedDestinationPoint.y();

            case Qt::DisplayRole:
              // truncate decimals for display
              return QString::number(
                       column == QgsGCPListModel::Column::DestinationX ? transformedDestinationPoint.x() : transformedDestinationPoint.y(),
                       'f', 4 );
            default:
              break;
          }
          break;
        }

        case QgsGCPListModel::Column::ResidualDx:
        case QgsGCPListModel::Column::ResidualDy:
        case QgsGCPListModel::Column::TotalResidual:
        {
          const double dX = point->residual().x();
          const double dY = point->residual().y();
          const double residual = std::sqrt( dX * dX + dY * dY );
          if ( !qgsDoubleNear( residual, 0 ) )
          {
            double value = 0;
            switch ( column )
            {
              case QgsGCPListModel::Column::ResidualDx:
                value = dX;
                break;
              case QgsGCPListModel::Column::ResidualDy:
                value = dY;
                break;
              case QgsGCPListModel::Column::TotalResidual:
                value = residual;
                break;
              default:
                break;
            }

            switch ( role )
            {
              case Qt::ToolTipRole:
              case Qt::EditRole:
              case Qt::UserRole:
                // use full precision
                return value;

              case Qt::DisplayRole:
                // truncate decimals for display
                return QString::number( value, 'f', 4 );
              default:
                break;
            }
          }
          else
          {
            return tr( "n/a" );
          }
          BUILTIN_UNREACHABLE
          break;
        }
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

    case Qt::TextAlignmentRole:
    {
      switch ( column )
      {
        case QgsGCPListModel::Column::Enabled:
          return Qt::AlignHCenter;

        case QgsGCPListModel::Column::LastColumn:
          break;

        case QgsGCPListModel::Column::ID:
        case QgsGCPListModel::Column::SourceX:
        case QgsGCPListModel::Column::SourceY:
        case QgsGCPListModel::Column::DestinationX:
        case QgsGCPListModel::Column::DestinationY:
        case QgsGCPListModel::Column::ResidualDx:
        case QgsGCPListModel::Column::ResidualDy:
        case QgsGCPListModel::Column::TotalResidual:
          return Qt::AlignRight;
      }
      break;
    }

    case static_cast< int >( Role::SourcePointRole ):
      return point->sourcePoint();

  }
  return QVariant();
}

bool QgsGCPListModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !mGCPList
       || index.row() < 0
       || index.row() >= mGCPList->size()
       || index.column() < 0
       || index.column() >= columnCount() )
    return false;

  QgsGeorefDataPoint *point = mGCPList->at( index.row() );
  const Column column = static_cast< Column >( index.column() );
  switch ( column )
  {
    case QgsGCPListModel::Column::Enabled:
      if ( role == Qt::CheckStateRole )
      {
        const bool checked = static_cast< Qt::CheckState >( value.toInt() ) == Qt::Checked;
        point->setEnabled( checked );
        emit dataChanged( index, index );
        updateResiduals();
        emit pointEnabled( point, index.row() );
        return true;
      }
      break;

    case QgsGCPListModel::Column::SourceX:
    case QgsGCPListModel::Column::SourceY:
    {
      QgsPointXY sourcePoint = point->sourcePoint();
      if ( column == QgsGCPListModel::Column::SourceX )
        sourcePoint.setX( value.toDouble() );
      else
        sourcePoint.setY( value.toDouble() );
      point->setSourcePoint( sourcePoint );
      emit dataChanged( index, index );
      updateResiduals();
      return true;
    }

    case QgsGCPListModel::Column::DestinationX:
    case QgsGCPListModel::Column::DestinationY:
    {
      QgsPointXY destinationPoint = point->destinationPoint();
      if ( column == QgsGCPListModel::Column::DestinationX )
        destinationPoint.setX( value.toDouble() );
      else
        destinationPoint.setY( value.toDouble() );
      point->setDestinationPoint( destinationPoint );
      emit dataChanged( index, index );
      updateResiduals();
      return true;
    }

    case QgsGCPListModel::Column::ID:
    case QgsGCPListModel::Column::ResidualDx:
    case QgsGCPListModel::Column::ResidualDy:
    case QgsGCPListModel::Column::TotalResidual:
    case QgsGCPListModel::Column::LastColumn:
      return false;
  }

  return false;
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

    case QgsGCPListModel::Column::SourceX:
    case QgsGCPListModel::Column::SourceY:
    case QgsGCPListModel::Column::DestinationX:
    case QgsGCPListModel::Column::DestinationY:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;

    case QgsGCPListModel::Column::ID:
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

void QgsGCPListModel::updateResiduals()
{
  if ( !mGCPList )
    return;

  mGCPList->updateResiduals( mGeorefTransform, mTargetCrs, mTransformContext, residualUnit() );
  emit dataChanged( index( 0, static_cast< int >( Column::ResidualDx ) ),
                    index( rowCount() - 1, static_cast< int >( Column::TotalResidual ) ) );
}


