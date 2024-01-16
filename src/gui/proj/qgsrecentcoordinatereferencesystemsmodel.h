/***************************************************************************
                    qgsrecentcoordinatereferencesystemsmodel.h
                    -------------------
    begin                : January 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H
#define QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>

class QgsCoordinateReferenceSystem;

/**
 * \class QgsRecentCoordinateReferenceSystemsModel
 * \ingroup core
 * \brief A model for display of recently used coordinate reference systems.
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsRecentCoordinateReferenceSystemsModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Custom roles used by the model
    enum Roles
    {
      RoleNodeType = Qt::UserRole, //!< Corresponds to the node's type
      RoleName = Qt::UserRole + 1, //!< The coordinate reference system name
      RoleAuthId = Qt::UserRole + 2, //!< The coordinate reference system authority name and id
      RoleDeprecated = Qt::UserRole + 3, //!< TRUE if the CRS is deprecated
      RoleType = Qt::UserRole + 4, //!< The coordinate reference system type
      RoleGroupId = Qt::UserRole + 5, //!< The node ID (for group nodes)
      RoleWkt = Qt::UserRole + 6, //!< The coordinate reference system's WKT representation. This is only used for non-standard CRS (i.e. those not present in the database).
      RoleProj = Qt::UserRole + 7, //!< The coordinate reference system's PROJ representation. This is only used for non-standard CRS (i.e. those not present in the database).
    };

    /**
     * Constructor for QgsRecentCoordinateReferenceSystemsModel, with the specified \a parent object.
     */
    QgsRecentCoordinateReferenceSystemsModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    /**
     * Returns the CRS for the corresponding \a index.
     *
     * Returns an invalid CRS if the index is not valid.
     */
    QgsCoordinateReferenceSystem crs( const QModelIndex &index ) const;

  private slots:

    void recentCrsPushed( const QgsCoordinateReferenceSystem &crs );
    void recentCrsRemoved( const QgsCoordinateReferenceSystem &crs );
    void recentCrsCleared();

  private:

    QList< QgsCoordinateReferenceSystem > mCrs;

};


#endif // QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H
