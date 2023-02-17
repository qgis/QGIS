/***************************************************************************
    qgsgcplistmodel.h - Model implementation of GCPList Model/View
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

#ifndef QGSGCP_LIST_TABLE_VIEW_H
#define QGSGCP_LIST_TABLE_VIEW_H

#include <QAbstractTableModel>
#include "qgis_app.h"
#include "qgsunittypes.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

class QgsGeorefDataPoint;
class QgsGeorefTransform;
class QgsGCPList;

class APP_EXPORT QgsGCPListModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    enum class Column : int
    {
      Enabled,
      ID,
      SourceX,
      SourceY,
      DestinationX,
      DestinationY,
      ResidualDx,
      ResidualDy,
      TotalResidual,
      LastColumn
    };

    enum class Role : int
    {
      SourcePointRole = Qt::UserRole + 1,
    };

    explicit QgsGCPListModel( QObject *parent = nullptr );

    void setGCPList( QgsGCPList *theGCPList );
    void setGeorefTransform( QgsGeorefTransform *georefTransform );

    /**
     * Sets the target (output) CRS for the georeferencing.
     */
    void setTargetCrs( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /**
     * Recalculates the residual values.
     */
    void updateResiduals();

    /**
     * Formats a number for display with an appropriate number of decimal places.
     */
    static QString formatNumber( double number );

  signals:

    void pointEnabled( QgsGeorefDataPoint *pnt, int i );

  private:
    QgsUnitTypes::RenderUnit residualUnit() const;

    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateTransformContext mTransformContext;

    QgsGCPList         *mGCPList = nullptr;
    QgsGeorefTransform *mGeorefTransform = nullptr;
};

#endif
