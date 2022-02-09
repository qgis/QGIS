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

#include <QTreeView>
#include <QAbstractTableModel>
#include "qgsunittypes.h"

class QgsGeorefDataPoint;
class QgsGeorefTransform;
class QgsGCPList;

class QgsGCPListModel : public QAbstractTableModel
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

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    void updateModel();
  private:
    QgsUnitTypes::RenderUnit residualUnit() const;

    QgsGCPList         *mGCPList = nullptr;
    QgsGeorefTransform *mGeorefTransform = nullptr;
};

#endif
